import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Users } from './users.page';

const routes: Routes = [{
	path: '',
	component: Users
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class UsersRoutingModule {}
