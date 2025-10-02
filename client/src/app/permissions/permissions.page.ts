import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-permissions',
	templateUrl: './permissions.page.html',
	styleUrls: ['./permissions.page.scss'],
	standalone: false
})

export class Permissions {
modules;
cards;
cardsOrig;
dResetSave: boolean = true;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

async ionViewDidEnter() {
	await this.getData();
}

compare(i) {
	if (this.cards[i]["bPublic"] != this.cardsOrig[i]["bPublic"])
		return false;
	if (this.cards[i]["bLocal"] != this.cardsOrig[i]["bLocal"])
		return false;
	if (this.cards[i]["bAdmin"] != this.cardsOrig[i]["bAdmin"])
		return false;
	if (this.cards[i]["bUser"] != this.cardsOrig[i]["bUser"])
		return false;
	return true;
}

compareAll() {
	let ret = true;
	for (let i = 0; i < this.cards.length; i++)
		if (this.compare(i) == false)
			ret = false;
	return ret;
}

update_(card) {
	if (card["bDisabled"]) {
		card["bPublic"] = false;
		card["bLocal"] = false;
		card["bAdmin"] = false;
		card["bUser"] = false;
		card["dPublic"] = true;
		card["dLocal"] = true;
		card["dAdmin"] = true;
		card["dUser"] = true;
	} else {
		card["dPublic"] = false;
		if (!card["bPublic"] && !card["bLocal"] && !card["bAdmin"] && !card["bUser"])
			card["bAdmin"] = true;
		if (card["bPublic"]) {
			card["bLocal"] = false;
			card["bAdmin"] = false;
			card["bUser"] = false;
			card["dLocal"] = true;
			card["dAdmin"] = true;
			card["dUser"] = true;
			return;
		}
		card["dLocal"] = false;
		card["dAdmin"] = false;
		card["dUser"] = false;
	}
}

update(c) {
	setTimeout(() => { this.update_(c); this.dResetSave = this.compareAll(); }, 1);
}

async save() {
	for (let i = 0; i < this.cards.length; i++)
		if (this.compare(i) == false) {
			const module = this.cards[i]["module"];
			if (this.modules[module] === undefined)
				this.modules[module] = {};
			if (this.cards[i]["bDisabled"]) {
				if (this.modules[module]["disabled"] !== this.cards[i]["bDisabled"])
					this.modules[module]["disabled"] = this.cards[i]["bDisabled"];
			} else {
				this.modules[module]["permissions"] = [];
				if (this.cards[i]["bPublic"])
					this.modules[module]["permissions"].push("_public_");
				if (this.cards[i]["bLocal"])
					this.modules[module]["permissions"].push("_localnetwork_");
				if (this.cards[i]["bAdmin"])
					this.modules[module]["permissions"].push("_admin_");
				if (this.cards[i]["buser"])
					this.modules[module]["permissions"].push("_user_");
			}
		}
	const ret = await this.httpClient.post("/MyDongleCloud/Auth/save-modules", JSON.stringify(this.modules), {headers:{"content-type": "application/json"}}).toPromise();
	console.log("Auth save-modules: ", ret);
	this.dResetSave = true;
}

async getData() {
	this.modules = this.global.session?.["modules"] ?? {};
	this.cards = [];
	const version = modulesDefault.version;
	delete modulesDefault.version;
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (value["web"] === true) {
			value["module"] = modulesMeta[key]["module"];
			value["title"] = modulesMeta[key]["title"];
			value["name"] = modulesMeta[key]["name"];
			value["description"] = modulesMeta[key]["description"];
			value["alias"] = [...(value["alias"] ?? []), ...(this.modules[key]?.alias ?? [])];
			const ll = value["alias"].length > 0 ? value["alias"][0] : key;
			value["link"] = (location.protocol + "//" + location.host + "/m/" + ll).toLowerCase();
			value["enabled"] = this.modules[key]?.enabled ?? value["enabled"] ?? true;
			value["permissions"] = this.modules[key]?.permissions ?? value["permissions"];
			for (let i = 0; i < value["permissions"].length; i++) {
				value["bDisabled"] = value["enabled"] == false;
				value["bPublic"] = value["permissions"][i] == "_public_";;
				value["bLocal"] = value["permissions"][i] == "_localnetwork_";
				value["bAdmin"] = value["permissions"][i] == "_admin_";
				value["bUser"] = value["permissions"][i] == "_user_";
			}
			this.cards.push(value);
			this.update_(value);
		}
	});
	this.cards.sort((a, b) => {
		return a["title"].localeCompare(b["title"]);
	});
	this.cardsOrig = structuredClone(this.cards);
	this.dResetSave = true;
}

}
