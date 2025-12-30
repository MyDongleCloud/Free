import { Component, ElementRef, Renderer2, AfterViewInit, OnDestroy, ChangeDetectorRef, Input } from '@angular/core';
import { Global } from '../../env';

@Component({
	selector: 'app-sidebar',
	templateUrl: './sidebar.component.html',
	standalone: false
})

export class SidebarComponent implements AfterViewInit, OnDestroy {
LG(st) { return this.global.mytranslateG(st); }
showMobileMenu: boolean = false;
@Input() public modulesTotal = null;
private documentClickListener: (() => void) | null = null;
cards = this.global.modulesData;
sidebarFilteredModules;

constructor(public global: Global, private cdr: ChangeDetectorRef, private elRef: ElementRef, private renderer: Renderer2) {
	this.sidebarFilterCards();
}

ngAfterViewInit() {
	this.documentClickListener = this.renderer.listen('document', 'click', (event: Event) => {
		const userMobileMenu = this.elRef.nativeElement.querySelector(".user-mobile-menu");
		const userMobileButton = this.elRef.nativeElement.querySelector(".user-mobile-button");
		if (this.showMobileMenu && userMobileMenu && userMobileButton && !userMobileMenu.contains(event.target) && !userMobileButton.contains(event.target)) {
			this.showMobileMenu = false;
			this.cdr.markForCheck();
		}
	});
}

ngOnDestroy() {
	if (this.documentClickListener) {
		this.documentClickListener();
		this.documentClickListener = null;
	}
}

refresh() {
	this.cdr.detectChanges();
}

sidebarFilterClick(type) {
	if (this.global.sidebarFilterType == type)
		this.global.sidebarFilterType = "";
}

sidebarFilterCards() {
	const term = this.global.sidebarSearchTerm.toLowerCase();
	this.sidebarFilteredModules = this.cards.filter(card => {
		if (this.global.sidebarFilterType == "Essential")
			return card.essential && card.web;
		else if (this.global.sidebarFilterType == "Bookmarks")
			return this.global.settings.bookmarks.includes(card.module);
		else if (this.global.sidebarFilterType == "Search")
			return this.global.sidebarSearchTerm == "" ? null : (card.module.toLowerCase().includes(term) || card.name.toLowerCase().includes(term) || card.title.toLowerCase().includes(term) || card.proprietary.some(pr => pr.toLowerCase().includes(term)) || card.keywords.some(kw => kw.toLowerCase().includes(term)));
		else
			return null;
	});
}

}
