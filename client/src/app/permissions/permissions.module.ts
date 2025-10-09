import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { ComponentsModule } from '../components/components.module';
import { PermissionsRoutingModule } from './permissions-routing.module';
import { Permissions } from './permissions.page';
import { FormsModule } from '@angular/forms';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		FormsModule,
		TranslateModule,
		ComponentsModule,
		PermissionsRoutingModule
	],
	declarations: [Permissions]
})

export class PermissionsModule {}
