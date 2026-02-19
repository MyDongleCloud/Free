import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { ACME_DIRECTORY_URLS, AcmeClient, AcmeOrder, CryptoKeyUtils } from "@fishballpkg/acme";
import { Global } from './env';

@Injectable({
	providedIn: "root"
})

export class Certificate {
SERVERURL: string = "https://mydongle.cloud";

constructor(private global: Global, private httpClient: HttpClient) {}

async process(name, shortname, customDomain) {
	const ret = { privateKey:"", fullChain:"" };
	const data = {};
	const domains = [ name + ".mydongle.cloud", "*." + name + ".mydongle.cloud", shortname + ".myd.cd", "*." + shortname + ".myd.cd" ];
	if (customDomain && customDomain != "") {
		domains.push(customDomain);
		domains.push("*." + customDomain);
	}
	const client = await AcmeClient.init(ACME_DIRECTORY_URLS.LETS_ENCRYPT_STAGING);
	const account = await client.createAccount({ emails: ["acme@mydongle.cloud"] });
	const order = await account.createOrder({ domains });
	const dns01Challenges = order.authorizations.map((authorization) => {
		return authorization.findDns01Challenge();
	});
	const expectedRecords = await Promise.all(
		dns01Challenges.map(async (challenge) => {
			const txtRecordContent = await challenge.digestToken();
			(data[challenge.authorization.domain.replace("*.", "")] ??= []).push(txtRecordContent);
			return true;
		})
	);
	this.global.consolelog(1, "CERTIFICATE: Data", data);
	const retA = await this.httpClient.post(this.SERVERURL + "/master/setup-certificate.json", "add=1&v=" + encodeURIComponent(JSON.stringify(data)), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	this.global.consolelog(1, "CERTIFICATE: Add", retA);
	await this.global.sleepms(5000);
	dns01Challenges.map(async (challenge) => {
		await challenge.submit()
	});
	await order.pollStatus({
		pollUntil: "ready",
		onBeforeAttempt: () => {},
		onAfterFailAttempt: () => { this.global.consolelog(1, "CERTIFICATE: After fail attempt"); }
	});
	const certKeyPair = await order.finalize();
	await order.pollStatus({ pollUntil: "valid" });
	const key = await CryptoKeyUtils.exportKeyPairToPem(certKeyPair);
	ret.privateKey = key.privateKey;
	ret.fullChain = await order.getCertificate();
	console.log(ret);
	const retD = await this.httpClient.post(this.SERVERURL + "/master/setup-certificate.json", "del=1&v=" + encodeURIComponent(JSON.stringify(data)), { headers:{ "content-type":"application/x-www-form-urlencoded" } }).toPromise();
	this.global.consolelog(1, "CERTIFICATE: Del", retD);
	return ret;
}

}
