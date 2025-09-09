import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import modulesDefault from '../modulesDefault.json';
import modulesMeta from '../modulesMeta.json';

@Component({
	selector: 'app-selection',
	templateUrl: './selection.page.html',
	styleUrls: ['./selection.page.scss'],
	standalone: false
})

export class Selection {
cards;
filteredCards;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection: "asc" | "desc" = "asc";
isTableView: string = "cards";

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});	
}

async ionViewDidEnter() {
	await this.getData();
}

async getData() {
	const modules = await this.httpClient.get("/assets/modules.json").toPromise();
	console.log(modules);
	this.cards = [];
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (value["web"] === true) {
			value["enabled"] = modules[key]?.enabled ?? true;
			value["authorized"] = modules[key]?.authorized ?? value["authorized"];
			value["alias"] = [...(value["alias"] ?? []), ...(modules[key]?.alias ?? [])];
			value["name"] = key;
			value["link"] = this.global.DONGLEURL + "/MyDongleCloud/" + value["name"];
			value["title"] = key;
			value["version"] = modulesMeta[key]?.version ?? "Not found";
			value["category"] = modulesMeta[key]?.category ?? "Not found";
			value["description"] = modulesMeta[key]?.description ?? "Not found";
			value["keywords"] = modulesMeta[key]?.keywords ?? [];
			this.cards.push(value);
		}
	});
	this.filteredCards = [...this.cards];
}

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter(card =>
		card.title.toLowerCase().includes(term) || card.description.toLowerCase().includes(term) || card.keywords.some(kw => kw.toLowerCase().includes(term))
	);
	this.sortCards();
}


sortCards() {
	this.filteredCards.sort((a, b) => {
		const aValue = a["name"];
		const bValue = b["name"];
		return this.sortDirection === "asc" ? aValue.localeCompare(bValue) : bValue.localeCompare(aValue);
	});
}

toggleSortDirection() {
	this.sortDirection = this.sortDirection === "asc" ? "desc" : "asc";
	this.sortCards();
}

toggleView(event: any) {
	this.isTableView = event.detail.value;
}

}
