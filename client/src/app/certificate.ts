import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Global } from './env';

@Injectable({
	providedIn: "root"
})

export class Certificate {
SERVERURL: string = "https://mydongle.cloud";

constructor(private global: Global, private httpClient: HttpClient) {}

async process(name, shortname, customDomain) {
	const ret = { accountKey:"", accountKeyId:"", fullChain:"", privateKey:"" };
	return ret;
}

}
