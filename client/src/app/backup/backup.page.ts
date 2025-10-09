import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-backup',
	templateUrl: 'backup.page.html',
	styleUrls: ['backup.page.scss'],
	standalone: false
})

export class Backup {

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

}
