import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Hardware } from './hardware.page';

const routes: Routes = [{
	path: '',
	component: Hardware
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class HardwareRoutingModule {}
