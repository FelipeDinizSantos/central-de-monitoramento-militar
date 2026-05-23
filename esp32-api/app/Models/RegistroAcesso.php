<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class RegistroAcesso extends Model
{
    protected $table = 'registros_acesso';

    protected $fillable = [
        'device_id',
        'imagem',
        'mensagem',
    ];
}