import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { JoyrideModule } from 'ngx-joyride';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { HomeRoutingModule } from './home-routing.module';
import { Home } from './home.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		JoyrideModule.forRoot(),
		FormsModule,
		TranslateModule,
		ComponentsModule,
		HomeRoutingModule
	],
	declarations: [Home]
})

export class HomeModule {}
