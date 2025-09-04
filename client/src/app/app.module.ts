import { NgModule, importProvidersFrom } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { RouteReuseStrategy } from '@angular/router';
import { IonicModule, IonicRouteStrategy } from '@ionic/angular';
import { AppComponent } from './app.component';
import { AppRoutingModule } from './app-routing.module';
import { provideHttpClient } from '@angular/common/http';
import { TranslateModule } from '@ngx-translate/core';
import { provideTranslateHttpLoader } from '@ngx-translate/http-loader';

@NgModule({
	declarations: [AppComponent],
	imports: [BrowserModule, IonicModule.forRoot(), AppRoutingModule],
	providers: [{ provide: RouteReuseStrategy, useClass: IonicRouteStrategy },
	provideHttpClient(),
	provideTranslateHttpLoader(),
	importProvidersFrom(
		TranslateModule.forRoot({
			defaultLanguage: "en",
		})
	),
],
  bootstrap: [AppComponent],
})
export class AppModule {}
