import { Component, ViewChild, ElementRef, HostListener, ChangeDetectorRef } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
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
LMT(st) { return this.global.mytranslateMT(st); }
LMD(st) { return this.global.mytranslateMD(st); }
@ViewChild("searchTermE") searchTermE: ElementRef;
modules;
cards;
cardsOrig;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection = { title:"asc", name:"asc", category:"asc", hits:"asc" };
dResetSave: boolean = true;
stats;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient, private route: ActivatedRoute) {
	this.route.queryParams.subscribe((params) => {
		if (params["search"]) {
			this.searchTerm = params?.["search"];
			setTimeout(() => {
				this.getData();
			}, 1);
		}
	});

	global.refreshUI.subscribe(event => {
		if (event === "modules")
			this.getData();
	});
	const data1 = localStorage.getItem("sortPropertyPermissions");
	if (data1)
		this.sortProperty = data1;
	const data2 = localStorage.getItem("sortDirectionPermissions");
	if (data2)
		this.sortDirection = JSON.parse(data2);
	this.getData();
}

private lastCtrlFPressTimestamp: number = 0;
@HostListener("document:keydown", ["$event"]) handleKeyboardEvent(event: KeyboardEvent) {
	const isCtrlF = (event.ctrlKey || event.metaKey) && event.key === "f";
	if (isCtrlF) {
		const currentTime = Date.now();
		const timeDifference = currentTime - this.lastCtrlFPressTimestamp;
		if (timeDifference < 5000) {
			this.lastCtrlFPressTimestamp = 0;
			this.global.dismissToast();
			this.searchTermE.nativeElement.blur();
			return;
		} else {
			this.lastCtrlFPressTimestamp = currentTime;
			event.preventDefault();
			this.searchTermE.nativeElement.focus();
			this.global.presentToast("Type Ctrl-F a second time for the browser search", "help-outline", 5000);
		}
	}
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

updatePerm_(card) {
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
		if (card["bUser"])
			card["bAdmin"] = true;
		card["dLocal"] = false;
		card["dAdmin"] = card["bUser"];
		card["dUser"] = false;
	}
}

updatePerm(c) {
	setTimeout(() => { this.updatePerm_(c); this.dResetSave = this.compareAll(); }, 1);
}

async save() {
	for (let i = 0; i < this.cards.length; i++)
		if (this.compare(i) == false) {
			const module = this.cards[i]["module"];
			if (this.modules[module] === undefined)
				this.modules[module] = {};
			if (this.cards[i]["bDisabled"]){
				this.modules[module]["enabled"] = false;
				delete this.modules[module]["permissions"];
			} else {
				delete this.modules[module]["enabled"];
				this.modules[module]["permissions"] = [];
				if (this.cards[i]["bPublic"])
					this.modules[module]["permissions"].push("_public_");
				if (this.cards[i]["bLocal"])
					this.modules[module]["permissions"].push("_localnetwork_");
				if (this.cards[i]["bUser"])
					this.modules[module]["permissions"].push("_groupuser_");
				else if (this.cards[i]["bAdmin"])
					this.modules[module]["permissions"].push("_groupadmin_");
				if (this.cards[i]["bDongle"])
					this.modules[module]["permissions"].push("_dongle_");
			}
		}
	const ret = await this.httpClient.post("/_app_/auth/module/permissions", JSON.stringify(this.modules), {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth modules-permissions: ", ret);
	this.dResetSave = true;
	await this.global.modulesDataPrepare();
	this.getData();
}

async getData(force = false) {
	if (!this.stats || force) {
		this.stats = await this.httpClient.post("/_app_/auth/module/stats", JSON.stringify({ all:true }), {headers:{"content-type": "application/json"}}).toPromise();
		this.global.consolelog(2, "Auth module-stats: ", this.stats);
	}
	if (this.global.modulesData.length == 0)
		return;
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
			value["bDisabled"] = value["enabled"] == false;
			value["bPublic"] = false;
			value["bLocal"] = false;
			value["bAdmin"] = false;
			value["bUser"] = false;
			value["bDongle"] = false;
			for (let i = 0; i < value["permissions"].length; i++) {
				value["bPublic"] ||= value["permissions"][i] == "_public_";;
				value["bLocal"] ||= value["permissions"][i] == "_localnetwork_";
				value["bAdmin"] ||= value["permissions"][i] == "_groupadmin_";
				value["bUser"] ||= value["permissions"][i] == "_groupuser_";
				value["bDongle"] ||= value["permissions"][i] == "_dongle_";
			}
			value["hits"] = this.stats?.[key]?.hits ?? 0;
			this.cards.push(value);
			this.updatePerm_(value);
		}
	});
	this.cards.sort((a, b) => {
		return a["title"].localeCompare(b["title"]);
	});
	this.cardsOrig = structuredClone(this.cards);
	this.dResetSave = true;
	this.filterCards();
}

validation(input, isFinalized, tokens, keywords) {
	return tokens.every((token, index) => {
		const isLastToken = index === tokens.length - 1;
		const isQuoted = input.includes(`"${token}"`);
		return keywords.some(kw => {
			if (isQuoted || !isLastToken || isFinalized)
				return kw === token || kw.split(" ").some(word => word === token);
			return kw.split(" ").some(word => word.startsWith(token));
		});
	});
}

filterCards(typing = false) {
	const input = this.searchTerm.toLowerCase();
	if (!input.trim())
		this.filteredCards = this.cards;
	else {
		const isFinalized = input.endsWith(" ");
		const tokens = [...input.toLowerCase().matchAll(/"([^"]+)"|(\S+)/g)].map(m => m[1] || m[2]);
		this.filteredCards = this.cards.filter(card => {
			return this.validation(input, isFinalized, tokens, card.hayStack);
		});
	}
	this.sortCards();
	if (this.searchTerm != "" && this.searchTermE)
		this.searchTermE.nativeElement.focus();
}

async updateHits() {
	this.global.presentToast("Data is being refreshed. Please wait...", "help-outline", 10000);
	const ret = await this.httpClient.get("/_app_/auth/refresh", {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth Refresh: ", ret);
	await this.global.sleepms(3000);
	await this.getData(true);
	this.global.dismissToast();
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
	localStorage.setItem("sortPropertyPermissions", this.sortProperty);
	localStorage.setItem("sortDirectionPermissions", JSON.stringify(this.sortDirection));
}

}
