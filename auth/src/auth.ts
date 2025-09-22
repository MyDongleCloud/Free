import { existsSync, readFileSync, writeFileSync } from "fs";
import { randomBytes } from 'crypto';
import { betterAuth } from "better-auth";
import { createAuthEndpoint, createAuthMiddleware } from "better-auth/api";
import { BetterAuthPlugin, customSession, jwt, oneTap, username, emailOTP, haveIBeenPwned } from "better-auth/plugins";
import { sendVerificationEmail, sendPasswordResetVerificationEmail, sendSignInOTP, sendVerificationEmailURL } from "./email";
import Database from "better-sqlite3";
import "dotenv/config";
import * as net from "net";
import * as jose from "jose";

export const port = 8091;
const adminPath = (process.env.PRODUCTION === "true" ? "" : "../rootfs") + "/disk/admin/.modules/";
const secretPath = adminPath + "BetterAuth/secret.txt";
const jwkPath = adminPath + "/MyDongleCloud/jwk.pub";
const databasePath = adminPath + "BetterAuth/database.sqlite";
const spacePath = adminPath + "MyDongleCloud/space.json";
let space = { name:"" };
if (existsSync(spacePath))
	space = JSON.parse(readFileSync(spacePath, "utf-8"));
const modulesPath = adminPath + "MyDongleCloud/modules.json";
let modules = {};
if (existsSync(modulesPath))
	modules = JSON.parse(readFileSync(modulesPath, "utf-8"));
const domain = process.env.PRODUCTION === "true" ? (space.name + ".mydongle.cloud") : "localhost";
const trustedOrigins = process.env.PRODUCTION === "true" ? [] : [ "http://localhost:8100" ];

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

const jwksPem = () => {
	return {
		id: "jwksPem",
		endpoints: {
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
	baseURL: "http://localhost:" + port + "/MyDongleCloud/Auth/",
	database: new Database(databasePath),
	emailAndPassword: {
		enabled: true
	},
	plugins: [
		username(),
		jwksPem(),
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
		oneTap(),
		customSession(async ({ user, session }) => {
			return {
				session,
				user,
				space,
				modules
			};
		}),
		jwt({
			jwt: {
				definePayload: ({ user }) => {
					return {
						role: "admin",
						user
					};
				}
			},
			jwks: {
				keyPairConfig: {
					alg: "ES256",
				},
			}
		})
	],
	 databaseHooks: {
		user: {
			create: {
				before: async (user, ctx) => {
					const countStatement = ctx?.context.options.database.prepare('SELECT COUNT(*) AS count FROM user');
					const result = countStatement.get();
					const un = result?.count == 0 ? "admin" : null;
					return { data: { ...user, username:un } };
				}
			}
		}
	},
	trustedOrigins: trustedOrigins,
	advanced: {
/*		crossSubDomainCookies: {
			enabled: true,
			domain: domain,
		}*/
	}
});

export const handler = auth.handler;
