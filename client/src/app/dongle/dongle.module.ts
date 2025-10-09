import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { FormsModule } from '@angular/forms';
import { DongleRoutingModule } from './dongle-routing.module';
import { Dongle } from './dongle.page';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		ComponentsModule,
		FormsModule,
		ComponentsModule,
		DongleRoutingModule
	],
	declarations: [Dongle]
})

export class DongleModule {}
