import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-backup',
	templateUrl: 'backup.page.html',
	styleUrls: ['backup.page.scss'],
	standalone: false
})

export class Backup {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

}
