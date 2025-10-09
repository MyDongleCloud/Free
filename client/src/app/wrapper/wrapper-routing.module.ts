import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { Wrapper } from './wrapper.page';

const routes: Routes = [{
	path: '',
	component: Wrapper,
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule]
})

export class WrapperRoutingModule {}
