import { Component, ViewChild, ElementRef, ChangeDetectorRef, ChangeDetectionStrategy, HostListener } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { JoyrideService } from 'ngx-joyride';
import { HttpClient } from '@angular/common/http';
import { SidebarComponent } from '../components/sidebar/sidebar.component';
import { Global } from '../env';
import { CategoriesBar } from '../myinterface';

@Component({
	selector: 'app-home',
	templateUrl: './home.page.html',
	changeDetection: ChangeDetectionStrategy.OnPush,
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
@ViewChild("searchTermE") searchTermE: ElementRef;
@ViewChild("sidebar") sidebar: any;
cardIdCur = 0;
cardConfig;
cards = this.global.modulesData;
filteredCards;
filteredGithubStars;
searchTerm: string = "";
sortProperty: string = "title";
sortDirection = { title:"asc", name:"asc", category:"asc" };
category: string = "All";
presentation: string = "cards";
showDetails: boolean = false;
showTerminal: boolean = false;
showDone: boolean = true;
showNotDone: boolean = true;
CategoriesBar = CategoriesBar;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient, private joyrideService: JoyrideService) {
	global.refreshUI.subscribe(event => {
		if (event === "onlySidebar") {
			this.sidebar.refresh();
			return;
		}
		this.filterCards();
		this.cdr.detectChanges();
	});
	this.filterCards();
	global.sidebarFilterType = "";
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

filterCards() {
	const term = this.searchTerm.toLowerCase();
	this.filteredCards = this.cards.filter(card => {
		if (this.showTerminal == false && card.web == false)
			return false;
		if (this.showDone == false && card.finished)
			return false;
		if (this.showNotDone == false && !card.finished)
			return false;
		let ret =  card.module.toLowerCase().includes(term) || card.name.toLowerCase().includes(term) || card.title.toLowerCase().includes(term) || card.proprietary.toLowerCase().includes(term) || card.keywords.some(kw => kw.toLowerCase().includes(term));
		if (this.category == "All")
			return ret;
		else if (this.category == "AI")
			return ret && card.ai;
		else if (this.category == "Essential")
			return ret && card.essential;
		else if (this.category == "Tag")
			return ret && this.global.settings.bookmarks.includes(card.module);
		else
			return ret && card.category.includes(this.category);
	});
	this.filteredGithubStars = Object.values(this.filteredCards).reduce((sum, card) => { return sum + (card["githubStars"] ?? 0); }, 0);
	this.sortCards();
	if (this.searchTerm != "")
		this.searchTermE.nativeElement.focus();
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
	this.global.settings.bookmarks = this.cards.filter(card => card.bookmark === true).map(card => card.module);//.join("|");
	this.global.settingsSave();
	if (this.category == "Tag")
		this.filterCards();
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

async moduleReset() {
	if (await this.global.presentQuestion("Reset \"" + this.cards[this.cardIdCur].title + "\" (" + this.cards[this.cardIdCur].name + ")", "WARNING! All data will be lost", "Are you sure to reset this module?"))
		if (await this.global.presentQuestion("Reset \"" + this.cards[this.cardIdCur].title + "\" (" + this.cards[this.cardIdCur].name + ")", "WARNING! All data will be lost", "This is your last chance. All data of this module will be erased and won't be recoverable. Are you absolutely sure to reset this module?")) {
			this.global.presentToast("The module is being resetted. Please wait...", "alert-circle-outline");
			const data = { module:this.global.modulesData[this.cardIdCur].module };
			const ret = await this.httpClient.post("/_app_/auth/module/reset", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
			this.global.consolelog(2, "Auth module-reset: ", ret);
			this.global.presentToast("The module has been resetted!", "alert-circle-outline");
			this.modalModuleSettings.dismiss();
		}
}

async moduleRefresh() {
	const data = { services:this.global.modulesData[this.cardIdCur].services };
	const ret = await this.httpClient.post("/_app_/auth/module/refresh", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	this.global.consolelog(2, "Auth module-refresh: ", ret);
	this.global.presentToast("The module service has been restarted!", "refresh-outline");
	this.modalModuleSettings.dismiss();
}

async config() {
	const data = { module:this.global.modulesData[this.cardIdCur].module };
	this.cardConfig = await this.httpClient.post("/_app_/auth/module/config", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	this.global.consolelog(2, "Auth module-config: ", this.cardConfig);
}

closeModuleSettings() {
	this.modalModuleSettings.dismiss();
}

ngOnInit() {
	if (!this.global.settings.welcomeTourShown) {
		setTimeout(() => {
		    this.joyrideService.startTour({
				steps: ["anchor1", "anchor2", "anchor3", "anchor4", "anchor5", "anchor6", "anchor7", "anchor8", "anchor9", "anchor10"]
			});
		}, 1000);
		this.global.settings.welcomeTourShown = true;
		this.global.settingsSave();
	}
}

}
