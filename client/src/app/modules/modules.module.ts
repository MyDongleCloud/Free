import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ModulesRoutingModule } from './modules-routing.module';
import { Modules } from './modules.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		TranslateModule,
		ModulesRoutingModule
	],
	declarations: [Modules]
})

export class ModulesModule {}
