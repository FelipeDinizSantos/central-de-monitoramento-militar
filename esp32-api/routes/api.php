<?php

use App\Http\Controllers\Api\RegistroAcessoController;
use Illuminate\Support\Facades\Route;

Route::prefix('registros-acesso')->group(function () {
    Route::post('/', [RegistroAcessoController::class, 'store']);
});