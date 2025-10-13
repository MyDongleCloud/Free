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
moduleConf;
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
	this.getData();
}

async getData() {
	this.modules = this.global.session?.["modules"] ?? {};
console.log(this.modules);
	this.cards = [];
	const version = modulesDefault.version;
	delete modulesDefault.version;
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (modulesMeta[key] === undefined) {
			console.log("Error: " + key + " not in modulesmeta");
			return;
		}
console.log(this.modules[key]?.enabled);
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
			value["link2"] = "https://" + ll + "." + (this.global.session?.["space"]?.["name"] ?? "") + ".mydongle.cloud";
			value["link"] = value["link"].toLowerCase();
			if (value["homepage"])
				value["link"] += value["homepage"];
			value["link2"] = value["link2"].toLowerCase();
		}
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
	this.cdr.markForCheck();
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

firstWords(st) {
	const words = st.split(" ");
	return words.slice(0, -1).join(" ");
}

lastWord(st) {
	const words = st.split(" ");
	return words[words.length - 1];
}

colorWord(st) {
	if (st == "_disabled_")
		return "bg-red-100 text-red-800";

	else if (st == "_public_")
		return "bg-green-100 text-green-800";
	else if (st == "_localnetwork_")
		return "bg-orange-100 text-orange-800";
	else if (st == "_groupadmin_")
		return "bg-yellow-100 text-yellow-800";
	else if (st == "_groupuser_")
		return "bg-purple-100 text-purple-800";

	else if (st == "Essential")
		return "bg-purple-100 text-purple-800";
	else if (st == "Personal")
		return "bg-blue-100 text-blue-800";
	else if (st == "Productivity")
		return "bg-indigo-100 text-indigo-800";
	else if (st == "Utils")
		return "bg-cyan-100 text-cyan-800";
	else if (st == "Developer")
		return "bg-red-100 text-red-800";

	else
		return "bg-gray-100 text-gray-800";
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
	if (this.cards[this.findIdByModule(this.moduleCur)].settings.conf)
		await this.conf();
	await this.modalModuleSettings.present();
}

async reset() {
	if (await this.global.presentQuestion("Reinitialize \"" + modulesDefault[this.moduleCur].title + "\" (" + modulesDefault[this.moduleCur].name + ")", "WARNING! All data will be lost", "Are you absolutely sure to reinitialize this module?")) {
		const data = { module:this.moduleCur };
		const ret = await this.httpClient.post("/MyDongleCloud/Auth/module-reset", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
		console.log("Auth module-reset: ", ret);
		this.modalModuleSettings.dismiss();
	}
}

async conf() {
	const data = { module:this.moduleCur };
	const ret = await this.httpClient.post("/MyDongleCloud/Auth/module-conf", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	console.log("Auth module-conf: ", ret);
	this.moduleConf = ret["conf"];
}

closeModuleSettings() {
	this.modalModuleSettings.dismiss();
}

}
