import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesDefault.json';
import modulesMeta from '../modulesMeta.json';

@Component({
	selector: 'app-home',
	templateUrl: './home.page.html',
	styleUrls: ['./home.page.scss'],
	standalone: false
})

export class Home {
@ViewChild("modalModuleSettings") modalModuleSettings: IonModal;
modules;
moduleCur;
cards;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection: "asc" | "desc" = "asc";
category: string = "All";
presentation: string = "cards";

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});	
}

async ionViewDidEnter() {
	await this.getData();
}

async getData() {
	try {
		this.modules = await this.httpClient.get("/data/modules.json").toPromise();
	} catch(e) {
		console.log("Failed to download /data/modules.json");
		this.modules = {};
	}
	this.cards = [];
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (value["web"] === true) {
			value["enabled"] = this.modules[key]?.enabled ?? true;
			value["permissions"] = this.modules[key]?.permissions ?? value["permissions"];
			value["alias"] = [...(value["alias"] ?? []), ...(this.modules[key]?.alias ?? [])];
			value["link"] = this.global.DONGLEURL + "/m/" + key;
			if (value["alias"].length > 0)
				value["link"] = this.global.DONGLEURL + "/m/" + value["alias"][0];
			Object.entries(modulesMeta[key]).forEach(([key2, value2]) => {
				value[key2] = value2;
			});
			this.cards.push(value);
		}
	});
	this.cards.sort((a, b) => {
		return a["title"].localeCompare(b["title"]);
	});
	this.filteredCards = [...this.cards];
}

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter( card => {
		if (this.category == "All")
			return card.title.toLowerCase().includes(term) || card.keywords.some(kw => kw.toLowerCase().includes(term));
		else
			return (card.title.toLowerCase().includes(term) || card.keywords.some(kw => kw.toLowerCase().includes(term))) && card.category.includes(this.category)
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

async settings(module) {
	this.moduleCur = module;
	await this.modalModuleSettings.present();
}

closeModuleSettings() {
	this.modalModuleSettings.dismiss();
}

}
