import { Component, Input, ElementRef, Renderer2, AfterViewInit, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { Global } from '../../env';

@Component({
	selector: 'app-topbar',
	templateUrl: './topbar.component.html',
	standalone: false
})

export class TopbarComponent implements AfterViewInit, OnDestroy {
LG(st) { return this.global.mytranslateG(st); }
@Input() title: string = "";
@Input() nameP: string = "";
private documentClickListener: (() => void) | null = null;
showUserMenu:boolean = false;
initials: string = "";

constructor(public global: Global, private cdr: ChangeDetectorRef, private elRef: ElementRef, private renderer: Renderer2) {}

toggleUserMenu() {
	this.showUserMenu = !this.showUserMenu;
}

getInitials() {
	const st = this.global.session?.user?.name ?? "";
	const nameParts = st.trim().split(" ");
	const initials = nameParts.map(name => name.charAt(0)).join("").toUpperCase();
	return initials;
}

ngAfterViewInit() {
	this.initials = this.getInitials();
	this.documentClickListener = this.renderer.listen("document", "click", (event: Event) => {
		const userMenuDropdown = this.elRef.nativeElement.querySelector(".user-menu-dropdown");
		const userMenuButton = this.elRef.nativeElement.querySelector(".user-menu-button");
		if (this.showUserMenu && userMenuDropdown && userMenuButton && !userMenuDropdown.contains(event.target) && !userMenuButton.contains(event.target)) {
			this.showUserMenu = false;
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

extract() {
	window.open((document.getElementById("iframeWrapper") as HTMLIFrameElement).src, "_blank");
}

}
