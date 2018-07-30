# frozen_string_literal: true

require 'spec_helper'

RSpec.describe Numo::Linalg do
  describe 'inv' do
    let(:m) { 5 }
    let(:mat_a) { rand_square_real_mat(m) }
    let(:mat_b) { rand_square_complex_mat(m) }

    it 'raises ArgumentError given a invalid driver option' do
      expect { described_class.inv(mat_a, driver: 'foo') }.to raise_error(ArgumentError)
    end

    it 'calculates the inverse of a square real matrix solving linear equation' do
      inv_mat_a = described_class.inv(mat_a, driver: 'gesv')
      expect((inv_mat_a.dot(mat_a) - Numo::DFloat.eye(m)).abs.max).to be < ERR_TOL
    end

    it 'calculates the inverse of a square complex matrix solving linear equation' do
      inv_mat_b = described_class.inv(mat_b, driver: 'gesv')
      expect((inv_mat_b.dot(mat_b) - Numo::DFloat.eye(m)).abs.max).to be < ERR_TOL
    end

    it 'calculates the inverse of a square real matrix using the LU factorization' do
      inv_mat_a = described_class.inv(mat_a, driver: 'getrf')
      expect((inv_mat_a.dot(mat_a) - Numo::DFloat.eye(m)).abs.max).to be < ERR_TOL
    end

    it 'calculates the inverse of a square complex matrix using the LU factorization' do
      inv_mat_b = described_class.inv(mat_b, driver: 'getrf')
      expect((inv_mat_b.dot(mat_b) - Numo::DFloat.eye(m)).abs.max).to be < ERR_TOL
    end
  end
end