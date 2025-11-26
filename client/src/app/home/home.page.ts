import { Component, ViewChild, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';
import { SidebarComponent } from '../components/sidebar/sidebar.component';

@Component({
	selector: 'app-home',
	templateUrl: './home.page.html',
	styleUrls: ['./home.page.scss'],
	standalone: false
})

export class Home {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
LK(st) { return this.global.mytranslateK(st); }
LMT(st) { return this.global.mytranslateMT(st); }
LMD(st) { return this.global.mytranslateMD(st); }
@ViewChild(SidebarComponent) sidebarComponent;
@ViewChild("modalModuleInfo") modalModuleInfo: IonModal;
@ViewChild("modalModuleSettings") modalModuleSettings: IonModal;
cardIdCur = 0;
cardConfig;
cards = this.global.modulesData;
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

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.filterCards();
	global.sidebarFilterType = "";
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
		return this.category == "All" ? ret : this.category == "ai" ? (ret && card.ai) : (ret && card.category.includes(this.category));
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
	this.cards[this.global.modulesDataFindId(m)]["bookmark"] = !this.cards[this.global.modulesDataFindId(m)]["bookmark"];
	this.global.settings.bookmarks.push(m);
	this.sidebarComponent.filterCards();
}

firstWords(st) {
	const words = st.split(" ");
	return words.slice(0, -1).join(" ");
}

lastWord(st) {
	const words = st.split(" ");
	return words[words.length - 1];
}

approximateStars(s) {
	return s > 10000 ? Math.round(s / 1000) : (s / 1000).toFixed(1);
}

async info(module) {
	this.cardIdCur = this.global.modulesDataFindId(module);
	await this.modalModuleInfo.present();
}

closeModuleInfo() {
	this.modalModuleInfo.dismiss();
}

async settings(module) {
	this.cardIdCur = this.global.modulesDataFindId(module);
	if (this.cards[this.cardIdCur].config)
		await this.config();
	await this.modalModuleSettings.present();
}

async reset() {
	if (await this.global.presentQuestion("Reset \"" + this.cards[this.cardIdCur].title + "\" (" + this.cards[this.cardIdCur].name + ")", "WARNING! All data will be lost", "Are you sure to reset this module?"))
		if (await this.global.presentQuestion("Reset \"" + this.cards[this.cardIdCur].title + "\" (" + this.cards[this.cardIdCur].name + ")", "WARNING! All data will be lost", "This is your last chance. All data of this module will be erased and won't be recoverable. Are you absolutely sure to reset this module?")) {
			const data = { module:this.global.modulesData[this.cardIdCur].module };
			const ret = await this.httpClient.post("/MyDongleCloud/Auth/module/reset", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
			this.global.consolelog(2, "Auth module-reset: ", ret);
			this.modalModuleSettings.dismiss();
		}
}

async config() {
	const data = { module:this.global.modulesData[this.cardIdCur].module };
	this.cardConfig = await this.httpClient.post("/MyDongleCloud/Auth/module/config", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	this.global.consolelog(2, "Auth module-config: ", this.cardConfig);
}

closeModuleSettings() {
	this.modalModuleSettings.dismiss();
}

}
