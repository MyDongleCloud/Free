import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule } from '@angular/forms';
import { ServerLogRoutingModule } from './serverlog-routing.module';
import { ServerLog } from './serverlog.page';
import { ComponentsModule } from '../components/components.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ComponentsModule,
		ServerLogRoutingModule
	],
	declarations: [ServerLog]
})

export class ServerLogModule {}
