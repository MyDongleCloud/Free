import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-permissions',
	templateUrl: './permissions.page.html',
	standalone: false
})

export class Permissions {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
LMT(st) { return this.global.mytranslateMT(st); }
LMD(st) { return this.global.mytranslateMD(st); }
modules;
cards;
cardsOrig;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection = { title:"asc", name:"asc", category:"asc", hits:"asc" };
dResetSave: boolean = true;
stats;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.getData();
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
					this.modules[module]["permissions"].push("_groupadmin_");
				if (this.cards[i]["buser"])
					this.modules[module]["permissions"].push("_groupuser_");
			}
		}
	const ret = await this.httpClient.post("/_app_/auth/module/permissions", JSON.stringify(this.modules), {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth modules-permissions: ", ret);
	this.dResetSave = true;
}

async getData() {
	const stats = await this.httpClient.post("/_app_/auth/module/stats", JSON.stringify({ all:true }), {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth module-stats: ", stats);
	this.modules = this.global.session?.["modules"] ?? {};
	this.cards = [];
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
				value["bAdmin"] = value["permissions"][i] == "_groupadmin_";
				value["bUser"] = value["permissions"][i] == "_groupuser_";
			}
			value["hits"] = stats?.[key]?.hits ?? 0;
			this.cards.push(value);
			this.update_(value);
		}
	});
	this.cards.sort((a, b) => {
		return a["title"].localeCompare(b["title"]);
	});
	this.cardsOrig = structuredClone(this.cards);
	this.dResetSave = true;
	this.filterCards();
}

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter( card => {
		let ret =  card.module.toLowerCase().includes(term) || card.name.toLowerCase().includes(term) || card.title.toLowerCase().includes(term) || card.proprietary.some(pr => pr.toLowerCase().includes(term)) || card.keywords.some(kw => kw.toLowerCase().includes(term));
		return ret;
	});
	this.sortCards();
}

async updateHits() {
	await this.httpClient.get("/_app_/auth/refresh", {headers:{"content-type": "application/json"}}).toPromise();
	await this.global.sleepms(3000);
	await this.getData();
}

sortCards() {
	this.filteredCards.sort((a, b) => {
		const aValue = a[this.sortProperty];
		const bValue = b[this.sortProperty];
		if (this.sortProperty == "hits")
			return (aValue - bValue) * (this.sortDirection[this.sortProperty] === "asc" ? 1 : -1);
		else
			return this.sortDirection[this.sortProperty] === "asc" ? aValue.localeCompare(bValue) : bValue.localeCompare(aValue);
	});
}

toggleSortDirection(p) {
	if (this.sortProperty == p)
		this.sortDirection[this.sortProperty] = this.sortDirection[this.sortProperty] === "asc" ? "desc" : "asc";
	this.sortProperty = p;
	this.sortCards();
}

}
