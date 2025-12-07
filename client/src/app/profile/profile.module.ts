import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { ProfileRoutingModule } from './profile-routing.module';
import { Profile } from './profile.page';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { QRCodeComponent } from 'angularx-qrcode';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		ReactiveFormsModule,
		QRCodeComponent,
		TranslateModule,
		ComponentsModule,
		ProfileRoutingModule
	],
	declarations: [Profile]
})

export class ProfileModule {}
