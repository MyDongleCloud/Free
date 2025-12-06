import { existsSync, readFileSync, writeFileSync } from "fs";
import { execSync } from "child_process";
import { randomBytes } from "crypto";
import { betterAuth, BetterAuthPlugin } from "better-auth";
import { APIError, createAuthEndpoint, createAuthMiddleware, sensitiveSessionMiddleware } from "better-auth/api";
import { username, customSession, emailOTP, magicLink, twoFactor, haveIBeenPwned, admin, jwt } from "better-auth/plugins";
import { sendMagicLinkEmail, sendVerificationEmail, sendPasswordResetVerificationEmail, sendSignInOTP, sendVerificationEmailURL } from "./email";
import { folderAndChildren } from "./tree";
import Database from "better-sqlite3";
import "dotenv/config";
import fs from "fs";
import * as net from "net";
import * as jose from "jose";
import * as os from "os";
import * as si from "systeminformation";

export const port = 8091;
const statusDemo = process.env.PRODUCTION === "true" ? false : true;
const adminPath = (process.env.PRODUCTION === "true" ? "" : "../rootfs") + "/disk/admin/modules/";
const version = readFileSync((process.env.PRODUCTION === "true" ? "" : "../rootfs") + "/usr/local/modules/mydonglecloud/version.txt", "utf-8");
const secretPath = adminPath + "betterauth/secret.txt";
const jwkPath = adminPath + "betterauth/jwk-pub.pem";
const databasePath = adminPath + "betterauth/database.sqlite";
const cloudPath = adminPath + "_config_/_cloud_.json";
export const cloud = JSON.parse(readFileSync(cloudPath, "utf-8"));
const modulesPath = adminPath + "_config_/_modules_.json";
let modules = {};
if (existsSync(modulesPath))
	modules = JSON.parse(readFileSync(modulesPath, "utf-8"));

function getInternalIpAddress() {
	const networkInterfaces = os.networkInterfaces();
	for (const name of Object.keys(networkInterfaces)) {
		const addresses = networkInterfaces[name];
		if (addresses)
			for (const address of addresses)
				if (address.family === "IPv4" && !address.internal)
					return address.address;
	}
	return undefined;
}
let trustedOrigins;
if (process.env.PRODUCTION === "true") {
	trustedOrigins = [ "*.mydongle.cloud", "*.mondongle.cloud", "*.myd.cd" ];
	if (cloud?.domains)
		cloud.all.domains.map( domain => trustedOrigins.push(`*.${domain}`) );
	const internalIP = getInternalIpAddress();
	internalIP && trustedOrigins.push(internalIP);
} else
	trustedOrigins = [ "http://localhost:8100" ];

if (!existsSync(secretPath)) {
	writeFileSync(secretPath, randomBytes(32).toString("base64"), "utf-8");
}

export const jwkInit = async () => {
	const response = await fetch("http://localhost:" + port + "/auth/jwks-pem");
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

si.networkStats();
let usersNb = 0;

const rootPath = "/var/log/";
const mdcEndpoints = () => {
	return {
		id: "mdcEndpoints",
		endpoints: {
			status: createAuthEndpoint("/status", {
				method: "GET",
			}, async(ctx) => {
				return Response.json({ status:"healthy", version, demo:statusDemo, timestamp:new Date().toISOString() }, { status:200 });
			}),

			update: createAuthEndpoint("/refresh", {
				method: "GET",
			}, async(ctx) => {
				sendToDongle({ a:"refresh" });
				return Response.json({ "status":"success" }, { status:200 });
			}),

			stats: createAuthEndpoint("/stats", {
				method: "GET",
			}, async(ctx) => {
				const ret = {};
				if (usersNb == 0) {
					const countStatement = ctx?.context.options.database.prepare("SELECT COUNT(*) AS count FROM user");
					const result = countStatement.get();
					usersNb = result?.count;
				}
				ret["users"] = { count: usersNb };
				const dataLoad = await si.currentLoad();
				ret["cpu"] = {
					current: {
						user: parseFloat(dataLoad.currentLoadUser.toFixed(2)), //%
						system: parseFloat(dataLoad.currentLoadSystem.toFixed(2)), //%
						usersystem: parseFloat((dataLoad.currentLoadUser + dataLoad.currentLoadSystem).toFixed(2)), //%
						idle: parseFloat(dataLoad.currentLoadIdle.toFixed(2)) //%
					},
					average: parseFloat(dataLoad.avgLoad.toFixed(2)) //%
				};
				const dataStorage_ = await si.fsSize();
				const dataStorage = dataStorage_.find(fs => fs.mount === "/");
				if (dataStorage)
					ret["storage"] = {
						usage: parseFloat(dataStorage.use.toFixed(2)), //%
						size: parseFloat((dataStorage.size / (1024 * 1024)).toFixed(0)), //MB
						used: parseFloat((dataStorage.used / (1024 * 1024)).toFixed(0)), //MB
						available: parseFloat((dataStorage.available / (1024 * 1024)).toFixed(0)) //MB
					};
				const dataNetwork_ = await si.networkStats();
				let dataNetwork;
				dataNetwork = dataNetwork_.find(nw => nw.iface === "wlan0");
				if (!dataNetwork)
					dataNetwork = dataNetwork_.find(nw => nw.iface === "eth0");
				if (!dataNetwork)
					dataNetwork = dataNetwork_.find(nw => nw.iface === "wlo1");
				if (dataNetwork)
					ret["network"] = {
						total: {
							rx: dataNetwork.rx_bytes ?? 0, //B
							tx: dataNetwork.tx_bytes ?? 0 //B
						},
						current: {
							rx: dataNetwork.rx_sec ?? 0, //B
							tx: dataNetwork.tx_sec ?? 0, //B
						}
					};
				return Response.json(ret, { status:200 });
			}),

			serverLog: createAuthEndpoint("/server-log", {
				method: "POST",
				use: [sensitiveSessionMiddleware]
			}, async(ctx) => {
				if (ctx.context.session?.user?.role != "admin")
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

			modulesPermissions: createAuthEndpoint("/module/permissions", {
				method: "POST",
			}, async(ctx) => {
				writeFileSync(modulesPath, JSON.stringify(ctx.body, null, "\t"), "utf-8");
				return Response.json({ "status":"success" }, { status:200 });
			}),

			moduleReset: createAuthEndpoint("/module/reset", {
				method: "POST",
			}, async(ctx) => {
				const tbe = "sudo /usr/local/modules/mydonglecloud/setup.sh " + ctx.body?.module;
				let ret;
				try {
					const output = execSync(tbe);
					console.log("Reset " + ctx.body?.module + ":\n", output);
					ret = { status:"success" };
				} catch (error) {
					ret = { status:"error" };
				}
				return Response.json(ret, { status:200 });
			}),

			moduleConfig: createAuthEndpoint("/module/config", {
				method: "POST",
			}, async(ctx) => {
				let ret;
				try {
					ret = JSON.parse(fs.readFileSync("/disk/admin/modules/_config_/" + ctx.body?.module + ".json", "utf8"));
				} catch (e) {
					ret = { status:"error" };
				}
				return Response.json(ret, { status:200 });
			}),

			moduleStats: createAuthEndpoint("/module/stats", {
				method: "POST",
			}, async(ctx) => {
				let ret = {};
				try {
					if (ctx.body?.all) {
						const files = fs.readdirSync("/var/log/apache2/");
						const matchingFiles = files.filter(file => /^stats-.*\.json$/.test(file));
						for (const file of matchingFiles)
							ret[file.replace("stats-", "").replace(".json", "")] = JSON.parse(fs.readFileSync("/var/log/apache2/" + file, "utf8"));
					} else
						ret = JSON.parse(fs.readFileSync("/var/log/apache2/stats-" + ctx.body?.module + ".json", "utf8"));
				} catch (e) {
					ret = { status:"error" };
				}
				return Response.json(ret, { status:200 });
			}),

			jwksPem: createAuthEndpoint("/jwks-pem", {
				method: "GET",
			}, async(ctx) => {
				const jwksRes = await fetch("http://localhost:" + port + "/auth/jwks");
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
	baseURL: "http://localhost:" + port + "/auth",
	database: new Database(databasePath),
	emailAndPassword: { enabled: true },
	advanced: { disableOriginCheck: process.env.PRODUCTION !== "true" },
	user: {
		deleteUser: {
			enabled: true,
			beforeDelete: async (user, request) => {
				if (user["username"] == cloud.all.name)
					throw new APIError("BAD_REQUEST", { message:"First account (username is the cloud name) can't be deleted" });
			}
		},
		additionalFields: {
			role: {
				type: "string",
				required: true,
				defaultValue: "user",
				input: false
			},
			settings: {
				type: "string",
				required: true,
				defaultValue: JSON.stringify({ lang:"en", powerUser:false, bookmarks:[], dontShowAgain:{} })
			}
		}
	},
	plugins: [
		username(),
		customSession(async ({ user, session }) => {
			return { session, user, cloud, modules };
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
				}
			}
		}),
		//haveIBeenPwned({ customPasswordCompromisedMessage:"Please choose a more secure password." }),
		//admin(),
		jwt({
			jwt: {
				expirationTime: "1w",
				definePayload: ({ user }) => {
					return {
						role: user["role"],
						username: user["username"],
						cloudname: cloud["name"],
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
						const cloudF = JSON.parse(readFileSync(cloudPath, "utf-8"));
						if (Object.keys(cloudF).length === 0) {
							console.log("PROBLEM: Creating first user but _cloud_.json is empty");
						}
						user.username = cloudF.all.name;
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
