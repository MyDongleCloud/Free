import { existsSync, readFileSync, writeFileSync } from "fs";
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

export const port = 8091;
const adminPath = (process.env.PRODUCTION === "true" ? "" : "../rootfs") + "/disk/admin/.modules/";
const secretPath = adminPath + "betterauth/secret.txt";
const jwkPath = adminPath + "/mydonglecloud/jwk.pub";
const databasePath = adminPath + "betterauth/database.sqlite";
const spacePath = adminPath + "mydonglecloud/space.json";
export const space = existsSync(spacePath) ? JSON.parse(readFileSync(spacePath, "utf-8")) : { name:"", domains: [] };
const modulesPath = adminPath + "mydonglecloud/modules.json";
let modules = {};
if (existsSync(modulesPath))
	modules = JSON.parse(readFileSync(modulesPath, "utf-8"));
const trustedOriginsDefault = [ "*.mydongle.cloud", "*.myd.cd" ];
const trustedOrigins = process.env.PRODUCTION === "true" ? (space?.domains ? [ ...trustedOriginsDefault, ...space?.domains.map(domain => `*.${domain}`)] : [ ...trustedOriginsDefault ]) : [ "http://localhost:8100" ];

if (!existsSync(secretPath)) {
	writeFileSync(secretPath, randomBytes(32).toString("base64"), "utf-8");
}

export const jwkInit = async () => {
	const response = await fetch("http://localhost:" + port + "/MyDongleCloud/Auth/jwks-pem");
	const fileContent = await response.text();
	writeFileSync(jwkPath, fileContent, "utf-8");
}

const sendOtpToDongle = (otp) => {
	const client = net.createConnection({ host: "127.0.0.1", port: 8093 });
	client.on("connect", () => {
		client.write(JSON.stringify({ a: "passcode", v: parseInt(otp) }));
		client.end();
	});
	client.on("end", () => {
	});
	client.on("error", (err: Error) => {
	});
}

const rootPath = "/work/ai.mydonglecloud/app/";
const mdcEndpoints = () => {
	return {
		id: "mdcEndpoints",
		endpoints: {
			serverLog: createAuthEndpoint("/server-log", {
				method: "POST",
				use: [sensitiveSessionMiddleware]
			}, async(ctx) => {
				if (ctx.context.session?.user?.username != "admin")
					return Response.json({}, { status: 200 });
				const myPath = rootPath + ctx.body?.path;
				const stats = fs.statSync(myPath);
				if (stats.isDirectory())
					return Response.json(folderAndChildren(myPath), { status: 200 });
				else
					return new Response(fs.readFileSync(myPath, "utf8"), { status: 200 });
			}),

			saveModules: createAuthEndpoint("/save-modules", {
				method: "POST",
			}, async(ctx) => {
				writeFileSync(modulesPath, JSON.stringify(ctx.body, null, "\t"), "utf-8");
				return Response.json({"success": true}, { status: 200 });
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
					throw new APIError("BAD_REQUEST", { message: "Admin account can't be deleted" });
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
				sendOtpToDongle(otp);
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
		//twoFactor(),
		//haveIBeenPwned({ customPasswordCompromisedMessage: "Please choose a more secure password." }),
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
			jwks: {
				keyPairConfig: {
					alg: "ES256",
				},
			}
		}),
		mdcEndpoints()
	],
	hooks: {
/*
		after: createAuthMiddleware(async (ctx) => {
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
		})
*/
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
