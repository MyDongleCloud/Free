import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Profile } from './profile.page';

const routes: Routes = [{
	path: '',
	component: Profile
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class ProfileRoutingModule {}
