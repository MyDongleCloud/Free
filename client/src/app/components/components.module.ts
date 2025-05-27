import { NgModule } from '@angular/core';
import { LongPressDirective } from './longPress/longPress.directive';

@NgModule({
	declarations: [LongPressDirective],
	exports: [LongPressDirective]
})
export class ComponentsModule {}
