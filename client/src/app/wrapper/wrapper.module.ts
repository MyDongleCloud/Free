import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { Wrapper } from './wrapper.page';
import { WrapperRoutingModule } from './wrapper-routing.module';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		ComponentsModule,
		WrapperRoutingModule
	],
	declarations: [Wrapper]
})

export class WrapperModule {}
