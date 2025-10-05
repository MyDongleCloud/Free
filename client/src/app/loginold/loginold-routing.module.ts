import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LoginOld } from './loginold.page';

const routes: Routes = [{
	path: '',
	component: LoginOld
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class LoginOldRoutingModule {}
