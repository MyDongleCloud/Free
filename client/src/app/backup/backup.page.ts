import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-backup',
	templateUrl: 'backup.page.html',
	standalone: false
})

export class Backup {
L(st) { return this.global.mytranslate(st); }

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

}
