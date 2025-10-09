import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { FormsModule } from '@angular/forms';
import { ServerLogRoutingModule } from './serverlog-routing.module';
import { ServerLog } from './serverlog.page';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		ComponentsModule,
		FormsModule,
		ComponentsModule,
		ServerLogRoutingModule
	],
	declarations: [ServerLog]
})

export class ServerLogModule {}
