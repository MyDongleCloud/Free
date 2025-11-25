import { Component, ChangeDetectorRef } from '@angular/core';
import { Location } from '@angular/common';
import { Router, NavigationStart, NavigationEnd, Event } from "@angular/router";
import { Global } from './env';


@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	styleUrls: ['app.component.scss'],
	standalone: false,
})

export class AppComponent {
LG(st) { return this.global.mytranslateG(st); }
navigationByBack: boolean = false;

constructor(public global: Global, private location: Location, public router: Router, private cdr: ChangeDetectorRef) {
	global.plt.backButton.subscribeWithPriority(10, () => {
		if (window.location.href.endsWith("/"))
			this.global.backButtonAlert();
		else
			this.location.back();
	});

	router.events.subscribe((event: Event) => {
		if (event instanceof NavigationEnd) {
			this.global.currentUrl = this.router.url.replace(/^\/|(\?).*$/g, "");
		}
	});
}

async changeLanguageAndRefresh(v: string) {
	await this.global.changeLanguage(v);
	this.cdr.detectChanges();
	setTimeout(() => {this.cdr.detectChanges();}, 500);
}

}
