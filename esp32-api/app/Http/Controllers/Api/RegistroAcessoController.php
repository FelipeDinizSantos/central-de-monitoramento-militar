<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\RegistroAcesso;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Storage;

class RegistroAcessoController extends Controller
{
    public function index(Request $request)
    {
        $query = RegistroAcesso::query();

        if ($request->data_inicial) {
            $query->whereDate('created_at','>=',$request->data_inicial, 'and');
        }

        if ($request->data_final) {
            $query->whereDate('created_at','<=', $request->data_final, 'and');
        }

        if ($request->hora_inicial) {
            $query->whereTime('created_at', '>=', $request->hora_inicial, 'and');
        }

        if ($request->hora_final) {
            $query->whereTime('created_at', '<=', $request->hora_final, 'and');
        }

        $registros = $query
            ->orderBy('id', 'desc')
            ->get();

        return view(
            'registros-acesso.index',
            compact('registros')
        );
    }

    public function store(Request $request)
    {
        try {

            $imageContent = $request->getContent();

            if (!$imageContent) {
                return response()->json([
                    'success' => false,
                    'message' => 'Imagem não recebida'
                ], 400);
            }

            $nomeImagem = time() . '.jpg';

            $caminhoImagem = 'fotos/' . $nomeImagem;

            Storage::disk('public')->put(
                $caminhoImagem,
                $imageContent
            );

            RegistroAcesso::create([
                'device_id' => $request->header('device-id'),
                'mensagem' => $request->header('mensagem'),
                'imagem' => $caminhoImagem,
            ]);

            return response()->json([
                'success' => true,
                'message' => 'Upload realizado'
            ]);

        } catch (\Exception $e) {

            return response()->json([
                'success' => false,
                'message' => $e->getMessage()
            ], 500);
        }
    }
}