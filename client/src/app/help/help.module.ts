import { NgModule, NO_ERRORS_SCHEMA } from '@angular/core';
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
	schemas: [NO_ERRORS_SCHEMA],
	declarations: [Help]
})

export class HelpModule {}
