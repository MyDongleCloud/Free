import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { Splash } from './splash.page';
import { SplashRoutingModule } from './splash-routing.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		SplashRoutingModule
	],
	declarations: [Splash]
})

export class SplashModule {}
