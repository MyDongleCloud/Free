import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-login',
	templateUrl: 'login.page.html',
	styleUrls: ['login.page.scss'],
	standalone: false
})

export class Login {

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

}
