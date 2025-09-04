import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-selection',
	templateUrl: './selection.page.html',
	styleUrls: ['./selection.page.scss'],
	standalone: false
})

export class Selection {

constructor(public global: Global, private cdr: ChangeDetectorRef) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

}
