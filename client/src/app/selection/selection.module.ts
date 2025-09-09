import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { SelectionRoutingModule } from './selection-routing.module';
import { Selection } from './selection.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		TranslateModule,
		SelectionRoutingModule
	],
	declarations: [Selection]
})

export class SelectionModule {}
