import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { SettingsRoutingModule } from './settings-routing.module';
import { Settings } from './settings.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		TranslateModule,
		ComponentsModule,
		SettingsRoutingModule
	],
	declarations: [Settings]
})

export class SettingsModule {}
