import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Global } from './env';
//import { toASCII } from 'punycode';
import * as ACME from '@root/acme';
import * as Keypairs from '@root/keypairs';
import * as CSR from '@root/csr';
import * as PEM from '@root/pem';
import * as ENC from '@root/encoding/base64';

@Injectable({
	providedIn: "root"
})

export class Certificate {
SERVERURL: string = "https://mydongle.cloud";

constructor(private global: Global, private httpClient: HttpClient) {}

async process(name, shortname, customDomain) {
	const ret = { accountKey:"", accountKeyId:"", fullChain:"", privateKey:"" };
	const STAGING = true;
	let acme = ACME.create({ maintainerEmail:"acme@@mydongle.cloud", packageAgent:"MDC/2026-01-01", notify:function (ev, msg) { console.log("AACMEE", msg); }, skipDryRun:true, skipChallengeTest:true });
	await acme.init("https://acme" + (STAGING ? "-staging" : "") + "-v02.api.letsencrypt.org/directory");

	const accountKeypair = await Keypairs.generate({ kty: "EC", format: "jwk" });
	const accountKey = accountKeypair.private;
	ret.accountKey = await Keypairs.export({ jwk: accountKey });
	this.global.consolelog(2, "ACME: Registering account...");
	const account = await acme.accounts.create({ subscriberEmail:"certificate@mydongle.cloud", agreeToTerms:true, accountKey:accountKey });
	this.global.consolelog(2, "ACME: Created account with id ", account.key.kid);
	ret.accountKeyId = account.key.kid;

	const serverKeypair = await Keypairs.generate({ kty: "RSA", format: "jwk" });
	const serverKey = serverKeypair.private;
	ret.privateKey = await Keypairs.export({ jwk: serverKey });
	const zones = [ name + ".mydongle.cloud", shortname + ".myd.cd" ];
	if (customDomain && customDomain != "")
		zones.push(customDomain);
	const domains = zones.flatMap(zone => [zone, `*.${zone}`]);
	const domainsNb = domains.length;
	this.global.consolelog(2, "ACME: Domains", domains);
	const csrDer = await CSR.csr({ jwk:serverKey, domains, encoding:"der" });
	const csr = PEM.packBlock({ type:"CERTIFICATE REQUEST", bytes:csrDer });
	const data = {};
	let dataIter = 0;
	const challenges = {
		"dns-01": {
			init: async (args) => {
				return null;
			},
			zones: async ({ challenge }) => {
				this.global.consolelog(2, "ACME: Zones", zones);
				return zones;
			},
			set: async ({ challenge }) => {
				const domain = (challenge?.wildcard ? "*." : "") + challenge.identifier.value;
				this.global.consolelog(2, "ACME: Set " + domain);
				//for (let i = 0; i < challenge.challenges.length; i++)
					//if (challenge.challenges[i]["type"] == "dns-01") {
						//this.global.consolelog(2, "ACME: Set" + (challenge?.wildcard ? " (.*)" : ""), challenge);
					//}
				const line = "_acme-challenge." + challenge.identifier.value + ". 1 IN TXT " + challenge.keyAuthorizationDigest + "\n";
				this.global.consolelog(2, line);
				(data[challenge.identifier.value] ??= []).push(challenge.keyAuthorizationDigest);
				dataIter++;
				if (domains.length == dataIter) {
					const retA = await this.httpClient.post(this.SERVERURL + "/master/setup-certificate.json", "add=1&v=" + encodeURIComponent(JSON.stringify(data)), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
					this.global.consolelog(2, "setup-certificate: add", retA);
					//alert("AuthorizationDigests sent to DNS server");
				}
			},
			get: async (args) => {
				//this.global.consolelog(2, "ACME: Get: ", args);
			},
			remove: async ({ challenge }) => {
				const domain = (challenge?.wildcard ? "*." : "") + challenge.identifier.value;
				this.global.consolelog(2, "ACME: Remove " + domain);
				//for (let i = 0; i < challenge.challenges.length; i++)
					//if (challenge.challenges[i]["type"] == "dns-01") {
						//this.global.consolelog(2, "ACME: Remove" + (challenge?.wildcard ? " (.*)" : ""), challenge);
					//}
			},
			propagationDelay: 5000
		}
	};
	try {
		const pems = await acme.certificates.create({ account, accountKey, csr, domains, challenges });
		ret.fullChain = pems.cert.trimEnd() + "\n" + pems.chain.trimEnd();
		this.global.consolelog(2, "##################################");
		this.global.consolelog(2, "ACME: Expiration " + pems.expires);
		this.global.consolelog(2, ret);
		this.global.consolelog(2, "##################################");
	} catch(e) {}
	const retD = await this.httpClient.post(this.SERVERURL + "/master/setup-certificate.json", "del=1&v=" + encodeURIComponent(JSON.stringify(data)), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	this.global.consolelog(2, "setup-certificate: del", retD);
	return ret;
}

}
