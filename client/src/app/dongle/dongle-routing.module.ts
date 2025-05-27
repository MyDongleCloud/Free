import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Dongle } from './dongle.page';

const routes: Routes = [{
	path: '',
	component: Dongle
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class DongleRoutingModule {}
