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

async process(name, shortname, additionalDomains) {
	const ret = { accountKey:"", accountKeyId:"", fullChain:"", privateKey:"" };
	const DOMAINNAME = ["mydongle.cloud", "mondongle.cloud", "myd.cd"];//.concat(additionalDomains);
	const STAGING = true;
	let acme = ACME.create({ maintainerEmail:"acme@@mydongle.cloud", packageAgent:"MDC/2025-01-01", notify:function (ev, msg) { /*this.global.consolelog(1, msg);*/ }, skipDryRun:true/*, skipChallengeTest:true*/ });
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

	const SUB = ["", "*."];
	let i = 1;
	const domains = DOMAINNAME.reduce((acc, dn) => {
		const subDomains = SUB.map(s => s + (i <= 2 ? (name + ".") : i <= 3 ? (shortname + ".") : "") + dn);
		i++;
		return acc.concat(subDomains);
	}, [] as string[]);
	const domainsNb = domains.length;
	this.global.consolelog(2, "ACME: Domains:" + domains.join(" "));
	const csrDer = await CSR.csr({ jwk:serverKey, domains, encoding:"der" });
	const csr = PEM.packBlock({ type:"CERTIFICATE REQUEST", bytes:csrDer });
	const lines = [];
	let domainIter = 0;
	const challenges = {
		"dns-01": {
			init: async (args) => {
				return null;
			},
			zones: async ({ challenge }) => {
				this.global.consolelog(2, "ACME: Zones", DOMAINNAME);
				return DOMAINNAME;
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
				lines.push(line);
				if (domains.length == lines.length) {
					const dataA = { lines, action:"add" };
					const retA = await this.httpClient.post(this.SERVERURL + "/master/certificates.json", dataA).toPromise();
					this.global.consolelog(2, retA);
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
			propagationDelay: 2000
		}
	};
	try {
		const pems = await acme.certificates.create({ account, accountKey, csr, domains, challenges });
		ret.fullChain = pems.cert + "\n" + pems.chain + "\n";
		this.global.consolelog(2, "##################################");
		this.global.consolelog(2, "ACME: Expiration " + pems.expires);
		this.global.consolelog(2, ret);
		this.global.consolelog(2, "##################################");
	} catch(e) {}
	const dataD = { lines, action:"delete" };
	const retD = await this.httpClient.post(this.SERVERURL + "/master/certificates.json", dataD).toPromise();
	this.global.consolelog(2, retD);
	return ret;
}

}
