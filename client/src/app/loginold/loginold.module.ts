import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { LoginOldRoutingModule } from './loginold-routing.module';
import { LoginOld } from './loginold.page';
import { ComponentsModule } from '../components/components.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ReactiveFormsModule,
		ComponentsModule,
		LoginOldRoutingModule
	],
	declarations: [LoginOld]
})

export class LoginOldModule {}
