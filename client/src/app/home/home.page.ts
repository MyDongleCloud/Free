import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesdefault.json';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-home',
	templateUrl: './home.page.html',
	styleUrls: ['./home.page.scss'],
	standalone: false
})

export class Home {
@ViewChild("modalModuleInfo") modalModuleInfo: IonModal;
modules;
moduleCur;
cards;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection: "asc" | "desc" = "asc";
category: string = "All";
presentation: string = "cards";
showDetails: boolean = true;
showNonWeb: boolean = false;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});	
}

async ionViewDidEnter() {
	await this.getData();
}

async getData() {
	this.modules = this.global.session?.["modules"] ?? {};
	this.cards = [];
	const version = modulesDefault.version;
	delete modulesDefault.version;
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (modulesMeta[key] === undefined) {
			console.log("Error: " + key + " not in modulesmeta");
			return;
		}
		if (value["web"] !== true)
			value = { permissions:["admin"], web:false, enabled:true };
		value["enabled"] = this.modules[key]?.enabled ?? value["enabled"] ?? true;
		value["permissions"] = this.modules[key]?.permissions ?? value["permissions"];
		value["alias"] = [...(value["alias"] ?? []), ...(this.modules[key]?.alias ?? [])];
		if (value["web"]) {
			const ll = value["alias"].length > 0 ? value["alias"][0] : key;
			value["link"] = location.protocol + "//" + location.host + "/m/" + ll;
			value["link2"] = "https://" + ll + "." + (this.global.session?.["space"]?.["name"] ?? "") + ".mydongle.cloud";
			value["link"] = value["link"].toLowerCase();
			value["link2"] = value["link2"].toLowerCase();
		}
		if (modulesMeta[key] !== undefined)
		Object.entries(modulesMeta[key]).forEach(([key2, value2]) => {
			value[key2] = value2;
		});
		value["keywords"].unshift(value["web"] ? "Web" : "Command-line");
		this.cards.push(value);
	});
	this.cards.sort((a, b) => {
		return a["title"].localeCompare(b["title"]);
	});
	this.filterCards();
}

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter( card => {
		if (this.showNonWeb == false && card.web == false)
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

filterNonWeb() {
	this.filterCards();
}

sortCards() {
	this.filteredCards.sort((a, b) => {
		const aValue = a[this.sortProperty];
		const bValue = b[this.sortProperty];
		return this.sortDirection === "asc" ? aValue.localeCompare(bValue) : bValue.localeCompare(aValue);
	});
}

toggleSortDirection(p) {
	this.sortProperty = p;
	this.sortDirection = this.sortDirection === "asc" ? "desc" : "asc";
	this.sortCards();
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

}
