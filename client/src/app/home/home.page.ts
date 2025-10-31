import { Component, ViewChild, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-home',
	templateUrl: './home.page.html',
	styleUrls: ['./home.page.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush,
	standalone: false
})

export class Home {
@ViewChild("modalModuleInfo") modalModuleInfo: IonModal;
@ViewChild("modalModuleSettings") modalModuleSettings: IonModal;
modules;
moduleCur;
moduleConfig;
cards;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection = { title:"asc", name:"asc", category:"asc" };
category: string = "All";
presentation: string = "cards";
showDetails: boolean = false;
showTerminal: boolean = false;
showDone: boolean = true;
showNotDone: boolean = true;
static firstTime = true;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.getData();
}

async getData() {
	this.modules = this.global.session?.["modules"] ?? {};
	this.cards = [];
	Object.entries(modulesMeta).forEach(([key, value]) => {
		if (modulesDefault[key] === undefined) {
			console.log("Error: " + key + " not in modulesdefault");
			return;
		}
	});
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (modulesMeta[key] === undefined) {
			console.log("Error: " + key + " not in modulesmeta");
			return;
		}
		value["enabled"] = this.modules[key]?.enabled ?? value["enabled"] ?? true;
		value["permissions"] = this.modules[key]?.permissions ?? value["permissions"];
		if (value["web"] !== true) {
			value["permissions"] = ["_groupadmin_"];
			value["web"] = false;
			value["enabled"] = true;
		}
		value["alias"] = [...(value["alias"] ?? []), ...(this.modules[key]?.alias ?? [])];
		if (value["web"]) {
			const ll = value["alias"].length > 0 ? value["alias"][0] : key;
			value["link"] = location.protocol + "//" + location.host + "/m/" + ll;
			value["link2"] = "https://" + ll + "." + (this.global.session?.["cloud"]?.["all"]?.["name"] ?? "") + ".mydongle.cloud";
			value["link"] = value["link"].toLowerCase();
			if (value["homepage"])
				value["link"] += value["homepage"];
			value["link2"] = value["link2"].toLowerCase();
		}
		Object.entries(modulesMeta[key]).forEach(([key2, value2]) => {
			value[key2] = value2;
		});
		if (Home.firstTime)
			value["keywords"].unshift(value["web"] ? "Web" : "Command-line");
		value["bookmark"] = false;
		this.cards.push(value);
	});
	this.filterCards();
	this.cdr.markForCheck();
	Home.firstTime = false;
}

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter( card => {
		if (this.showTerminal == false && card.web == false)
			return false;
		if (this.showDone == false && card.status == "done")
			return false;
		if (this.showNotDone == false && card.status != "done")
			return false;
		let ret =  card.module.toLowerCase().includes(term) || card.name.toLowerCase().includes(term) || card.title.toLowerCase().includes(term) || card.proprietary.some(pr => pr.toLowerCase().includes(term)) || card.keywords.some(kw => kw.toLowerCase().includes(term));
		return this.category == "All" ? ret : (ret && card.category.includes(this.category));
	});
	this.sortCards();
}

filterCategory(c) {
	this.category = c;
	this.filterCards();
}

sortCards() {
	this.filteredCards.sort((a, b) => {
		const aValue = a[this.sortProperty];
		const bValue = b[this.sortProperty];
		return this.sortDirection[this.sortProperty] === "asc" ? aValue.localeCompare(bValue) : bValue.localeCompare(aValue);
	});
}

toggleSortDirection(p) {
	if (this.sortProperty == p)
		this.sortDirection[this.sortProperty] = this.sortDirection[this.sortProperty] === "asc" ? "desc" : "asc";
	this.sortProperty = p;
	this.sortCards();
}

bookmark(m) {
	this.cards[this.findIdByModule(m)]["bookmark"] = !this.cards[this.findIdByModule(m)]["bookmark"];
}

firstWords(st) {
	const words = st.split(" ");
	return words.slice(0, -1).join(" ");
}

lastWord(st) {
	const words = st.split(" ");
	return words[words.length - 1];
}

findIdByModule(m) {
	let ret = 0;
	this.cards.forEach((card, index) => {
		if (card["module"] == m)
			ret = index;
	});
	return ret;
}

approximateStars(s) {
	return s > 10000 ? Math.round(s / 1000) : (s / 1000).toFixed(1);
}

async info(module) {
	this.moduleCur = module;
	await this.modalModuleInfo.present();
}

closeModuleInfo() {
	this.modalModuleInfo.dismiss();
}

async settings(module) {
	this.moduleCur = module;
	if (this.cards[this.findIdByModule(this.moduleCur)].reset)
		await this.config();
	await this.modalModuleSettings.present();
}

async reset() {
	if (await this.global.presentQuestion("Reset \"" + this.cards[this.findIdByModule(this.moduleCur)].title + "\" (" + this.cards[this.findIdByModule(this.moduleCur)].name + ")", "WARNING! All data will be lost", "Are you sure to reset this module?"))
		if (await this.global.presentQuestion("Reset \"" + this.cards[this.findIdByModule(this.moduleCur)].title + "\" (" + this.cards[this.findIdByModule(this.moduleCur)].name + ")", "WARNING! All data will be lost", "This is your last chance. All data of this module will be erased and won't be recoverable. Are you absolutely sure to reset this module?")) {
			const data = { module:this.moduleCur };
			const ret = await this.httpClient.post("/MyDongleCloud/Auth/module/reset", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
			console.log("Auth module-reset: ", ret);
			this.modalModuleSettings.dismiss();
		}
}

async config() {
	const data = { module:this.moduleCur };
	this.moduleConfig = await this.httpClient.post("/MyDongleCloud/Auth/module/config", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	console.log("Auth module-config: ", this.moduleConfig);
}

closeModuleSettings() {
	this.modalModuleSettings.dismiss();
}

}
