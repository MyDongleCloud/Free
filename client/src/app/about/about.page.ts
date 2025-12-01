import { Component, ChangeDetectorRef } from '@angular/core';
import { BleService } from '../ble';
import { Global } from '../env';

@Component({
	selector: 'app-about',
	templateUrl: 'about.page.html',
	standalone: false
})

export class About {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
isDevCheckbox: boolean;

constructor(public global: Global, private cdr: ChangeDetectorRef, public ble: BleService) {}

async longCopyright() {
}

}
