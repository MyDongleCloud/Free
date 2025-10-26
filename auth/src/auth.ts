import { existsSync, readFileSync, writeFileSync } from "fs";
import { execSync } from 'child_process';
import { randomBytes } from "crypto";
import { betterAuth } from "better-auth";
import { APIError, createAuthEndpoint, createAuthMiddleware, sensitiveSessionMiddleware } from "better-auth/api";
import { BetterAuthPlugin, username, customSession, emailOTP, magicLink, twoFactor, haveIBeenPwned, admin, jwt } from "better-auth/plugins";
import { sendMagicLinkEmail, sendVerificationEmail, sendPasswordResetVerificationEmail, sendSignInOTP, sendVerificationEmailURL } from "./email";
import { folderAndChildren } from "./tree";
import Database from "better-sqlite3";
import "dotenv/config";
import fs from "fs";
import * as net from "net";
import * as jose from "jose";
import * as os from 'os';

export const port = 8091;
const statusDemo = process.env.PRODUCTION === "true" ? false : true;
const adminPath = (process.env.PRODUCTION === "true" ? "" : "../rootfs") + "/disk/admin/modules/";
const secretPath = adminPath + "betterauth/secret.txt";
const jwkPath = adminPath + "betterauth/jwk-pub.pem";
const databasePath = adminPath + "betterauth/database.sqlite";
const spacePath = adminPath + "mydonglecloud/space.json";
export const space = existsSync(spacePath) ? JSON.parse(readFileSync(spacePath, "utf-8")) : { name:"", domains: [] };
const modulesPath = adminPath + "mydonglecloud/modules.json";
let modules = {};
if (existsSync(modulesPath))
	modules = JSON.parse(readFileSync(modulesPath, "utf-8"));

function getInternalIpAddress() {
	const networkInterfaces = os.networkInterfaces();
	for (const name of Object.keys(networkInterfaces)) {
		const addresses = networkInterfaces[name];
		if (addresses)
			for (const address of addresses)
				if (address.family === 'IPv4' && !address.internal)
					return address.address;
	}
	return undefined;
}
let trustedOrigins;
if (process.env.PRODUCTION === "true") {
	trustedOrigins = [ "*.mydongle.cloud", "*.myd.cd" ];
	if (space?.domains)
		space.domains.map( domain => trustedOrigins.push(`*.${domain}`) );
	const internalIP = getInternalIpAddress();
	internalIP && trustedOrigins.push(internalIP);
} else
	trustedOrigins = [ "http://localhost:8100" ];

if (!existsSync(secretPath)) {
	writeFileSync(secretPath, randomBytes(32).toString("base64"), "utf-8");
}

export const jwkInit = async () => {
	const response = await fetch("http://localhost:" + port + "/MyDongleCloud/Auth/jwks-pem");
	const fileContent = await response.text();
	writeFileSync(jwkPath, fileContent, "utf-8");
}

const sendToDongle = (data) => {
	const client = net.createConnection({ host:"127.0.0.1", port:8093 });
	client.on("connect", () => {
		client.write(JSON.stringify(data));
		client.end();
	});
	client.on("end", () => {
	});
	client.on("error", (err: Error) => {
	});
}

const rootPath = "/var/log/";
const mdcEndpoints = () => {
	return {
		id: "mdcEndpoints",
		endpoints: {
			status: createAuthEndpoint("/status", {
				method: "GET",
			}, async(ctx) => {
				return Response.json({ status:"healthy", demo:statusDemo, timestamp:new Date().toISOString() }, { status:200 });
			}),

			serverLog: createAuthEndpoint("/server-log", {
				method: "POST",
				use: [sensitiveSessionMiddleware]
			}, async(ctx) => {
				if (ctx.context.session?.user?.username != "admin")
					return Response.json({}, { status: 200 });
				let path = ctx.body?.path;
				path = path.replace(/\/\/+/g, "/");
				while (path.includes("/../"))
					path = path.replace(/\/[^/]+\/\.\.\//g, "/");
				path = path.replace(/^\/\.\.\/|\/\.\.\/?$/, "");
				const fullPath = rootPath + path;
				const stats = fs.statSync(fullPath);
				if (stats.isDirectory())
					return Response.json(folderAndChildren(fullPath), { status: 200 });
				else
					return new Response(fs.readFileSync(fullPath, "utf8"), { status: 200 });
			}),

			modulesPermissions: createAuthEndpoint("/modules-permissions", {
				method: "POST",
			}, async(ctx) => {
				writeFileSync(modulesPath, JSON.stringify(ctx.body, null, "\t"), "utf-8");
				sendToDongle({ a:"update" });
				return Response.json({ success:true }, { status:200 });
			}),

			moduleReset: createAuthEndpoint("/module/reset", {
				method: "POST",
			}, async(ctx) => {
				const tbe = "sudo /usr/local/modules/mydonglecloud/setup.sh " + ctx.body?.module;
				let ret;
				try {
					const output = execSync(tbe);
					console.log("Output:\n", output);
					ret = '{ "status":"success" }';
				} catch (error) {
					ret = '{ "status":"error" }';
				}
				return Response.json(JSON.parse(ret), { status:200 });
			}),

			moduleConfig: createAuthEndpoint("/module/config", {
				method: "POST",
			}, async(ctx) => {
				let ret;
				try {
					ret = fs.readFileSync("/disk/admin/modules/_config_/" + ctx.body?.module + ".json", "utf8");
				} catch (e) {
					ret = '{ "status":"error" }';
				}
				return Response.json(JSON.parse(ret), { status:200 });
			}),

			jwksPem: createAuthEndpoint("/jwks-pem", {
				method: "GET",
			}, async(ctx) => {
				const jwksRes = await fetch("http://localhost:" + port + "/MyDongleCloud/Auth/jwks");
				const jwks = await jwksRes.json();
				const key = await jose.importJWK(jwks.keys[0], "ES256") as jose.KeyObject;
				const pem = await jose.exportSPKI(key);
				return new Response(pem, { status: 200 });
			})
		}
	} satisfies BetterAuthPlugin
}

export const auth = betterAuth({
	secret: readFileSync(secretPath, "utf-8").trim(),
	baseURL: "http://localhost:" + port + "/MyDongleCloud/Auth",
	database: new Database(databasePath),
	emailAndPassword: { enabled: true },
	user: {
		deleteUser: {
			enabled: true,
			beforeDelete: async (user, request) => {
				if (user["username"] == "admin")
					throw new APIError("BAD_REQUEST", { message:"Admin account can't be deleted" });
			}
		},
		additionalFields: {
			role: {
				type: "string",
				required: true,
				defaultValue: "user",
				input: false
			},
			lang: {
				type: "string",
				required: true,
				defaultValue: "en"
			}
		}
	},
	plugins: [
		username(),
		customSession(async ({ user, session }) => {
			return { session, user, space, modules };
		}),
		emailOTP({
			async sendVerificationOTP({ email, otp, type }) {
				sendToDongle({ a:"passcode", v:parseInt(otp) });
				if (type === "email-verification") {
					await sendVerificationEmail(email, otp);
				} else if (type === "sign-in") {
					await sendSignInOTP(email, otp);
				} else if (type === "forget-password") {
					await sendPasswordResetVerificationEmail(email, otp);
				}
			},
		}),
		magicLink({
			async sendMagicLink({ email, token, url }) {
				await sendMagicLinkEmail(email, token, url);
			},
		}),
		twoFactor({
			skipVerificationOnEnable:true,
			otpOptions: {
				async sendOTP({ user, otp }, request) {
					sendToDongle({ a:"otp", v:parseInt(otp), e:user?.["email"] });
					console.log(otp);
				}
			}
		}),
		//haveIBeenPwned({ customPasswordCompromisedMessage:"Please choose a more secure password." }),
		//admin(),
		jwt({
			jwt: {
				expirationTime: "1d",
				definePayload: ({ user }) => {
					return {
						role: "admin",
						username: user["username"],
						spacename: space["name"],
						user
					};
				}
			},
			jwks: { keyPairConfig:{ alg:"ES256" } }
		}),
		mdcEndpoints()
	],
	hooks: {
/*
		before: createAuthMiddleware(async (ctx) => {
			console.log("???????????????????????????");
			//console.log(ctx.headers);
			console.log(ctx.body);
			console.log("???????????????????????????");
		}),
*/
		after: createAuthMiddleware(async (ctx) => {
/*
			console.log("###########################");
			const responseBody = ctx.context.returned;
			let logBody;
			if (responseBody instanceof Response) {
				const status = responseBody.status || 200;
				const bodyText = await responseBody.text();
				logBody = bodyText.startsWith("{") || bodyText.startsWith("[") ? JSON.stringify(JSON.parse(bodyText), null, 2) : bodyText;
				console.log(`${status} for ${ctx.method} ${ctx.path}:\n`, logBody);
			} else {
				logBody = typeof responseBody === "string" ? responseBody : JSON.stringify(responseBody, null, 2);
				console.log(`200 for ${ctx.method} ${ctx.path}:\n`, logBody);
			}
			console.log("###########################");
*/
			if (ctx.path == "/two-factor/verify-otp" && ctx.context.returned?.["statusCode"] === undefined)
				sendToDongle({ a:"otp", v:0 });
		})
	},
	databaseHooks: {
		user: {
			create: {
				before: async (user, ctx) => {
					const countStatement = ctx?.context.options.database.prepare("SELECT COUNT(*) AS count FROM user");
					const result = countStatement.get();
					if (result?.count == 0) {
						user.username = "admin";
						user.role = "admin";
					}
					return { data: { ...user } };
				}
			}
		}
	},
	trustedOrigins: trustedOrigins
});

export const handler = auth.handler;
