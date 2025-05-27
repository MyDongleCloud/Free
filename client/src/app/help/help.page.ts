import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-help',
	templateUrl: 'help.page.html',
	styleUrls: ['help.page.scss'],
})

export class Help {

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

}
