import { Component, ChangeDetectorRef } from '@angular/core';
import { Router } from '@angular/router';
import { Location } from '@angular/common';
import { Global } from './env';


@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	standalone: false
})

export class AppComponent {
LG(st) { return this.global.mytranslateG(st); }
navigationByBack: boolean = false;

constructor(public global: Global, public router: Router, private location: Location, private cdr: ChangeDetectorRef) {
	global.plt.backButton.subscribeWithPriority(10, () => {
		if (window.location.href.endsWith("/"))
			this.global.backButtonAlert();
		else
			this.location.back();
	});
}

async changeLanguageAndRefresh(v: string) {
	await this.global.changeLanguage(v);
	this.cdr.detectChanges();
	setTimeout(() => {this.cdr.detectChanges();}, 500);
}

}
