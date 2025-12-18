import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { FormsModule } from '@angular/forms';
import { HardwareRoutingModule } from './hardware-routing.module';
import { Hardware } from './hardware.page';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		ComponentsModule,
		FormsModule,
		ComponentsModule,
		HardwareRoutingModule
	],
	declarations: [Hardware]
})

export class HardwareModule {}
