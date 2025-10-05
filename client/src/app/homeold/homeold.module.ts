import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { HomeOldRoutingModule } from './homeold-routing.module';
import { HomeOld } from './homeold.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		TranslateModule,
		HomeOldRoutingModule
	],
	declarations: [HomeOld]
})

export class HomeOldModule {}
