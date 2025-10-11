import { Component, ElementRef, Renderer2, AfterViewInit, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { Global } from '../../env';

@Component({
	selector: 'app-sidebar',
	templateUrl: './sidebar.component.html',
	styleUrls: ['./sidebar.component.scss'],
	standalone: false
})

export class SidebarComponent implements AfterViewInit, OnDestroy {
showMobileMenu: boolean = false;
private documentClickListener: (() => void) | null = null;

constructor(public global: Global, private cdr: ChangeDetectorRef, private elRef: ElementRef, private renderer: Renderer2) {}

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

}
