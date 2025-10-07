import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SetupRoutingModule } from './setup-routing.module';
import { Setup } from './setup.page';
import { ComponentsModule } from '../components/components.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ReactiveFormsModule,
		ComponentsModule,
		SetupRoutingModule
	],
	declarations: [Setup]
})

export class SetupModule {}
