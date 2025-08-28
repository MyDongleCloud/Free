import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { Help } from './help.page';
import { HelpRoutingModule } from './help-routing.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		HelpRoutingModule
	],
	declarations: [Help]
})

export class HelpModule {}
