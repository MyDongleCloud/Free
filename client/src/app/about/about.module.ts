import { NgModule, NO_ERRORS_SCHEMA } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule } from '@angular/forms';
import { AboutRoutingModule } from './about-routing.module';
import { About } from './about.page';
import { ComponentsModule } from '../components/components.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ComponentsModule,
		AboutRoutingModule
	],
	schemas: [NO_ERRORS_SCHEMA],
	declarations: [About]
})

export class AboutModule {}
