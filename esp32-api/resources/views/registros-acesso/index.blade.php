@extends('layouts.app')
@section('title', 'Central de Monitoramento')
@section('styles')

<style>

    body{
        background: #1b1f1d;
        color: #d7dadc;
    }

    .top-area{
        background:
            linear-gradient(rgba(15,23,15,.92), rgba(15,23,15,.95)),
            url('https://images.unsplash.com/photo-1511884642898-4c92249e20b6?q=80&w=1600&auto=format&fit=crop');
        background-size: cover;
        background-position: center;
        padding: 45px 0;
        border-bottom: 2px solid #556b2f;
        box-shadow: 0 4px 20px rgba(0,0,0,.5);
    }

    .top-title{
        font-size: 2rem;
        font-weight: 700;
        letter-spacing: 1px;
        color: #d9e4c8;
        text-transform: uppercase;
    }

    .top-subtitle{
        opacity: .8;
        color: #aab49f;
    }

    .filter-card{
        background: #2a2f2b;
        border: 1px solid #556b2f;
        border-radius: 18px;
        padding: 20px;
        margin-bottom: 30px;
        box-shadow: 0 4px 20px rgba(0,0,0,.25);
    }

    .form-control{
        background: #1f2421;
        border: 1px solid #556b2f;
        color: #d7dadc;
    }

    .form-control:focus{
        background: #1f2421;
        color: white;
        border-color: #7c9a45;
        box-shadow: none;
    }

    .btn-military{
        background: #556b2f;
        color: white;
        border: none;
        font-weight: 600;
        letter-spacing: .5px;
        transition: .3s;
    }

    .btn-military:hover{
        background: #6b8440;
        color: white;
    }

    .camera-card{
        border: 1px solid #556b2f;
        border-radius: 18px;
        overflow: hidden;
        transition: .3s;
        background: #2a2f2b;
        box-shadow: 0 4px 15px rgba(0,0,0,.3);
    }

    .camera-card:hover{
        transform: translateY(-5px);
        box-shadow: 0 10px 30px rgba(0,0,0,.45);
    }

    .camera-image{
        width: 100%;
        height: 260px;
        object-fit: cover;
        cursor: pointer;
        border-bottom: 1px solid #556b2f;
    }

    .badge-device{
        background: #556b2f;
        color: white;
        font-size: .75rem;
        padding: 7px 10px;
        border-radius: 30px;
    }

    .empty-state{
        padding: 100px 20px;
        text-align: center;
        color: #9da39d;
    }

    .empty-icon{
        font-size: 80px;
        margin-bottom: 20px;
        color: #556b2f;
    }

    .modal-content{
        background: #1f2421;
        color: white;
    }

    .modal-header{
        border-bottom: 1px solid #556b2f;
    }

    .modal-image{
        width: 100%;
        border-radius: 10px;
    }

    .info-box{
        background: #1f2421;
        border: 1px solid #556b2f;
        border-radius: 12px;
        padding: 10px 15px;
        font-size: .9rem;
    }

</style>

@endsection

@section('content')

<div class="top-area">
    <div class="container">
        <div class="d-flex justify-content-between align-items-center flex-wrap gap-3">
            <div>
                <div class="top-title">
                    Central de Monitoramento
                </div>

                <div class="top-subtitle">
                    Sistema de vigilância ESP32-CAM
                </div>

            </div>

            <div class="info-box">
                <div>
                    <strong>Total de registros:</strong>
                    {{ $registros->count() }}
                </div>
            </div>
        </div>
    </div>
</div>

<div class="container py-4">
    <div class="filter-card">
        <form method="GET" action="{{ url()->current() }}">
            <div class="row g-3 align-items-end">
                <div class="col-md-3">
                    <label class="form-label">
                        Data Inicial
                    </label>

                    <input
                        type="date"
                        name="data_inicial"
                        class="form-control"
                        value="{{ request('data_inicial') }}"
                    >
                </div>

                <div class="col-md-3">
                    <label class="form-label">
                        Data Final
                    </label>

                    <input
                        type="date"
                        name="data_final"
                        class="form-control"
                        value="{{ request('data_final') }}"
                    >
                </div>

                <div class="col-md-2">
                    <label class="form-label">
                        Hora Inicial
                    </label>

                    <input
                        type="time"
                        name="hora_inicial"
                        class="form-control"
                        value="{{ request('hora_inicial') }}"
                    >
                </div>

                <div class="col-md-2">
                    <label class="form-label">
                        Hora Final
                    </label>

                    <input
                        type="time"
                        name="hora_final"
                        class="form-control"
                        value="{{ request('hora_final') }}"
                    >
                </div>

                <div class="col-md-2 d-grid">
                    <button
                        type="submit"
                        class="btn btn-military">

                        <i class="bi bi-search"></i>
                        Buscar
                    </button>

                </div>
            </div>
        </form>
    </div>

    @if($registros->count())
        <div class="row g-4">
            @foreach($registros as $registro)
                <div class="col-md-6 col-lg-4">
                    <div class="card camera-card h-100">
                        <img
                            src="{{ asset('storage/' . $registro->imagem) }}"
                            class="camera-image preview-image"
                            data-image="{{ asset('storage/' . $registro->imagem) }}"
                        >

                        <div class="card-body">
                            <div class="d-flex justify-content-between align-items-center mb-3">
                                <span class="badge-device">
                                    <i class="bi bi-cpu"></i>
                                    {{ $registro->device_id ?? 'SEM DEVICE ID' }}
                                </span>

                                <small class="text-light">
                                    #{{ $registro->id }}
                                </small>
                            </div>

                            <div class="mb-3 text-light">
                                <i class="bi bi-clock-history"></i>
                                {{ $registro->created_at->format('d/m/Y H:i:s') }}
                            </div>

                            @if($registro->mensagem)
                                <div class="alert border mb-0" style="background:#1f2421; border-color:#556b2f !important; color:#d7dadc;">
                                    <i class="bi bi-broadcast"></i>
                                    {{ $registro->mensagem }}
                                </div>
                            @endif
                        </div>
                    </div>
                </div>
            @endforeach
        </div>
    @else
        <div class="empty-state">
            <div class="empty-icon">
                <i class="bi bi-camera"></i>
            </div>
            <h3>
                Nenhum registro encontrado
            </h3>
            <p>
                Nenhuma captura encontrada para o período informado.
            </p>
        </div>
    @endif
</div>

<div class="modal fade" id="imageModal" tabindex="-1">
    <div class="modal-dialog modal-xl modal-dialog-centered">
        <div class="modal-content border-0">
            <div class="modal-header">
                <h5 class="modal-title">
                    Visualização da Imagem
                </h5>

                <button
                    type="button"
                    class="btn-close btn-close-white"
                    data-bs-dismiss="modal">
                </button>

            </div>

            <div class="modal-body text-center">
                <img
                    src=""
                    id="modalImage"
                    class="modal-image"
                >
            </div>
        </div>
    </div>
</div>

@endsection

@section('scripts')

<script>

    $(document).on('click', '.preview-image', function () {

        let image = $(this).data('image');

        $('#modalImage').attr('src', image);

        $('#imageModal').modal('show');

    });

</script>

@endsection