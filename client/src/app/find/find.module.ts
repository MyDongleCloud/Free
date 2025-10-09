import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IonicModule } from '@ionic/angular';
import { TranslateModule } from '@ngx-translate/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { FindRoutingModule } from './find-routing.module';
import { Find } from './find.page';

@NgModule({
	imports: [
		CommonModule,
		IonicModule,
		TranslateModule,
		FormsModule,
		ReactiveFormsModule,
		FindRoutingModule
	],
	declarations: [Find]
})

export class FindModule {}
