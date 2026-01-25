import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SetupRoutingModule } from './setup-routing.module';
import { Setup } from './setup.page';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ReactiveFormsModule,
		SetupRoutingModule
	],
	declarations: [Setup]
})

export class SetupModule {}
