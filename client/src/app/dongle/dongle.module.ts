import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule } from '@angular/forms';
import { DongleRoutingModule } from './dongle-routing.module';
import { Dongle } from './dongle.page';
import { ComponentsModule } from '../components/components.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ComponentsModule,
		DongleRoutingModule
	],
	declarations: [Dongle]
})

export class DongleModule {}
