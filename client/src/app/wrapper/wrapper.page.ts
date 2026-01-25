import { Component, ChangeDetectorRef } from '@angular/core';
import { DomSanitizer, SafeResourceUrl } from '@angular/platform-browser';
import { ActivatedRoute } from '@angular/router';
import { Global } from '../env';
import modulesMeta from '../modulesmeta.json';

@Component({
	selector: 'app-wrapper',
	templateUrl: 'wrapper.page.html',
	standalone: false
})

export class Wrapper {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
LMT(st) { return this.global.mytranslateMT(st); }
module: string = "";
subdomain: string = "";
page: string = "";
title: string = "";
finished: boolean = true;
safeUrl: SafeResourceUrl;

constructor(public global: Global, private route: ActivatedRoute, private sanitizer: DomSanitizer, private cdr: ChangeDetectorRef) {
	this.route.queryParams.subscribe((params) => {
		this.module = params?.module;
		this.subdomain = params?.subdomain;
		this.page = params?.page;
		this.title = "Wrapper: " + this.LMT(modulesMeta[this.module].title) + " (" + modulesMeta[this.module].name + ")";
		this.finished = modulesMeta[this.module].finished;
		if (!this.global.demo && (this.global.developer || this.finished))
			this.update();
	});
}

update() {
	const tmp = this.subdomain ?? this.module;
	const url = location.protocol + "//" + location.host + "/m/" + tmp + (this.page ?? "");
	this.safeUrl = this.sanitizer.bypassSecurityTrustResourceUrl(url);
}

}
